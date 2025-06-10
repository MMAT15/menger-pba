import React, { useState, useEffect, useMemo, createContext, useContext } from 'react';
import { initializeApp } from 'firebase/app';
import { 
    getFirestore, 
    collection, 
    addDoc, 
    onSnapshot, 
    query, 
    doc,
    getDocs,
    getDoc,
    where,
    runTransaction
} from 'firebase/firestore';
import { 
    getAuth, 
    signInAnonymously, 
    onAuthStateChanged,
    signInWithCustomToken
} from 'firebase/auth';
import { LineChart, Line, XAxis, YAxis, Tooltip, ResponsiveContainer, CartesianGrid } from 'recharts';

// --- Íconos SVG para una mejor UI ---
const PlusIcon = () => (<svg xmlns="http://www.w3.org/2000/svg" className="h-6 w-6" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 6v6m0 0v6m0-6h6m-6 0H6" /></svg>);
const SearchIcon = () => (<svg xmlns="http://www.w3.org/2000/svg" className="h-5 w-5 text-gray-400" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z" /></svg>);
const SparkleIcon = (props) => (<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" {...props}><path fillRule="evenodd" d="M10 2a1 1 0 011 1v2.586l1.293-1.293a1 1 0 111.414 1.414L12.414 7H15a1 1 0 110 2h-2.586l1.293 1.293a1 1 0 11-1.414 1.414L11 9.414V12a1 1 0 11-2 0V9.414l-1.293 1.293a1 1 0 01-1.414-1.414L7.586 8H5a1 1 0 110-2h2.586L6.293 4.707a1 1 0 011.414-1.414L9 4.586V2a1 1 0 011-1zM3.293 3.293a1 1 0 011.414 0L6 4.586V6a1 1 0 11-2 0V4.586L2.707 3.293a1 1 0 010-1.414zM14 6a1 1 0 012 0v1.414l1.293-1.293a1 1 0 111.414 1.414L17.414 8H18a1 1 0 110 2h-.586l1.293 1.293a1 1 0 11-1.414 1.414L16 11.414V14a1 1 0 11-2 0v-2.586l-1.293 1.293a1 1 0 11-1.414-1.414L12.586 10H14a1 1 0 110-2h-1.414l1.293-1.293a1 1 0 011.414 0z" clipRule="evenodd" /></svg>);
const StarIcon = ({ className }) => (<svg xmlns="http://www.w3.org/2000/svg" className={className} viewBox="0 0 20 20" fill="currentColor"><path d="M9.049 2.927c.3-.921 1.603-.921 1.902 0l1.07 3.292a1 1 0 00.95.69h3.462c.969 0 1.371 1.24.588 1.81l-2.8 2.034a1 1 0 00-.364 1.118l1.07 3.292c.3.921-.755 1.688-1.54 1.118l-2.8-2.034a1 1 0 00-1.175 0l-2.8 2.034c-.784.57-1.838-.197-1.539-1.118l1.07-3.292a1 1 0 00-.364-1.118L2.98 8.72c-.783-.57-.38-1.81.588-1.81h3.461a1 1 0 00.951-.69l1.07-3.292z" /></svg>);
const HomeIcon = () => <svg xmlns="http://www.w3.org/2000/svg" className="h-6 w-6" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M3 12l2-2m0 0l7-7 7 7M5 10v10a1 1 0 001 1h3m10-11l2 2m-2-2v10a1 1 0 01-1 1h-3m-6 0a1 1 0 001-1v-4a1 1 0 011-1h2a1 1 0 011 1v4a1 1 0 001 1m-6 0h6" /></svg>;
const ChartBarIcon = () => <svg xmlns="http://www.w3.org/2000/svg" className="h-5 w-5 mr-1" viewBox="0 0 20 20" fill="currentColor"><path fillRule="evenodd" d="M3 3a1 1 0 000 2v8a1 1 0 001 1h2.586l-1.293 1.293a1 1 0 101.414 1.414L10 12.414l3.293 3.293a1 1 0 001.414-1.414L13.414 14H16a1 1 0 001-1V5a1 1 0 10-2 0v5h-1.586l-1.293-1.293a1 1 0 10-1.414 1.414L12 11.586V5a1 1 0 00-1-1H4a1 1 0 00-1-1z" clipRule="evenodd" /></svg>;

// --- Sistema de Notificaciones (Toast) ---
const ToastContext = createContext();
const useToast = () => useContext(ToastContext);

const ToastProvider = ({ children }) => {
    const [toasts, setToasts] = useState([]);
    const addToast = (message, type = 'success') => {
        const id = Date.now();
        setToasts(prev => [...prev, { id, message, type }]);
        setTimeout(() => {
            setToasts(prev => prev.filter(t => t.id !== id));
        }, 3000);
    };

    return (
        <ToastContext.Provider value={addToast}>
            {children}
            <div className="fixed bottom-5 right-5 z-50 space-y-2">
                {toasts.map(toast => (
                    <div key={toast.id} className={`px-4 py-2 rounded-lg shadow-lg text-white animate-fade-in-out ${toast.type === 'success' ? 'bg-green-500' : 'bg-red-500'}`}>
                        {toast.message}
                    </div>
                ))}
            </div>
        </ToastContext.Provider>
    );
};


// --- Configuración de Firebase ---
const firebaseConfig = typeof __firebase_config !== 'undefined' ? JSON.parse(__firebase_config) : {};
const appId = typeof __app_id !== 'undefined' ? __app_id : 'default-app-id';

// --- Inicialización de Firebase ---
const app = initializeApp(firebaseConfig);
const db = getFirestore(app);
const auth = getAuth(app);

// --- Componente de Rating con Estrellas ---
const StarRating = ({ rating, onRating, readOnly = false, isSubmitting = false }) => {
    return (
        <div className="flex items-center">
            {[1, 2, 3, 4, 5].map((star) => (
                <StarIcon 
                    key={star} 
                    className={`h-7 w-7 ${rating >= star ? 'text-yellow-400' : 'text-gray-300'} ${!readOnly && 'cursor-pointer hover:text-yellow-300 transition-colors'} ${isSubmitting && 'opacity-50 cursor-not-allowed'}`}
                    onClick={() => !readOnly && !isSubmitting && onRating(star)} 
                />
            ))}
        </div>
    );
};

// --- Componente principal de la App ---
export default function App() {
    return (
        <ToastProvider>
            <MengerApp />
        </ToastProvider>
    );
}

function MengerApp() {
    const [user, setUser] = useState(null);
    const [products, setProducts] = useState([]);
    const [prices, setPrices] = useState({});
    const [view, setView] = useState('home');
    const [selectedProduct, setSelectedProduct] = useState(null);
    const [loading, setLoading] = useState(true);
    const [searchTerm, setSearchTerm] = useState('');

    useEffect(() => {
        const authAndLoad = async () => {
            try {
                if (typeof __initial_auth_token !== 'undefined' && __initial_auth_token) {
                    await signInWithCustomToken(auth, __initial_auth_token);
                } else {
                    await signInAnonymously(auth);
                }
            } catch (error) { console.error("Authentication error:", error); }
        };
        const unsubscribeAuth = onAuthStateChanged(auth, currentUser => {
            setUser(currentUser);
        });
        authAndLoad();
        return () => unsubscribeAuth();
    }, []);

    useEffect(() => {
        if (!user) return;
        setLoading(true); // Start loading when user is available
        const productsQuery = query(collection(db, `artifacts/${appId}/public/data/products`));
        const unsubscribeProducts = onSnapshot(productsQuery, snapshot => {
            const productsData = snapshot.docs.map(doc => ({ id: doc.id, ...doc.data() }));
            setProducts(productsData);
            setLoading(false);
        }, error => {
            console.error("Error fetching products:", error);
            setLoading(false);
        });

        const pricesQuery = query(collection(db, `artifacts/${appId}/public/data/prices`));
        const unsubscribePrices = onSnapshot(pricesQuery, snapshot => {
            const pricesData = {};
            snapshot.forEach(doc => {
                const data = doc.data();
                if (!pricesData[data.productId]) pricesData[data.productId] = [];
                pricesData[data.productId].push({ id: doc.id, ...data });
            });
            setPrices(pricesData);
        }, error => console.error("Error fetching prices:", error));

        return () => { unsubscribeProducts(); unsubscribePrices(); };
    }, [user]);

    const navigateTo = (newView, data = null) => {
        setView(newView);
        setSelectedProduct(data);
    };

    const filteredProducts = products.filter(p =>
        (p.name?.toLowerCase() || '').includes(searchTerm.toLowerCase()) ||
        (p.brand?.toLowerCase() || '').includes(searchTerm.toLowerCase())
    );

    const getProductStats = (productId) => {
        const productPrices = prices[productId] || [];
        if (productPrices.length === 0) return { count: 0, minPrice: null };
        const minPrice = Math.min(...productPrices.map(p => p.price));
        return { count: productPrices.length, minPrice };
    };

    return (
        <div className="bg-gray-100 min-h-screen font-sans">
            <Header onNavigate={navigateTo} currentView={view} />
            <main className="p-4 md:p-6 max-w-5xl mx-auto">
                {view === 'home' && <ProductList products={filteredProducts} loading={loading} getProductStats={getProductStats} onNavigate={navigateTo} onSearch={setSearchTerm} />}
                {view === 'addProduct' && <AddProductForm onNavigate={navigateTo} />}
                {view === 'productDetail' && <ProductDetail product={selectedProduct} prices={prices[selectedProduct?.id] || []} onNavigate={navigateTo} user={user} />}
                {view === 'analysis' && <SmartAnalysisView products={products} prices={prices} />}
            </main>
        </div>
    );
}

function Header({ onNavigate, currentView }) {
    return (
        <header className="bg-white shadow-sm sticky top-0 z-20">
            <div className="max-w-5xl mx-auto px-4 py-3 flex justify-between items-center">
                <h1 onClick={() => onNavigate('home')} className="text-2xl font-bold text-indigo-600 cursor-pointer">Menger en PBA</h1>
                <nav className="flex items-center space-x-2 md:space-x-4">
                    <button onClick={() => onNavigate('home')} title="Inicio" className={`p-2 rounded-full ${currentView === 'home' ? 'bg-indigo-100 text-indigo-600' : 'text-gray-500 hover:bg-gray-100'}`}><HomeIcon /></button>
                    <button onClick={() => onNavigate('analysis')} title="Análisis IA" className={`p-2 rounded-lg flex items-center transition-colors text-sm font-semibold ${currentView === 'analysis' ? 'bg-indigo-100 text-indigo-600' : 'text-gray-500 hover:bg-gray-100'}`}>
                        <SparkleIcon className="h-5 w-5 mr-0 md:mr-2" />
                        <span className="hidden md:inline">Análisis IA</span>
                    </button>
                    <button onClick={() => onNavigate('addProduct')} title="Agregar Producto" className="bg-indigo-500 hover:bg-indigo-600 text-white font-bold py-2 px-3 rounded-lg flex items-center transition-colors">
                        <PlusIcon />
                        <span className="hidden md:inline ml-2">Producto</span>
                    </button>
                </nav>
            </div>
        </header>
    );
}

const ProductCardSkeleton = () => (
    <div className="bg-white p-4 rounded-xl shadow-md border border-gray-200 animate-pulse">
        <div className="h-6 bg-gray-200 rounded w-3/4 mb-2"></div>
        <div className="h-4 bg-gray-200 rounded w-1/2 mb-4"></div>
        <div className="mt-3 pt-3 border-t border-gray-100 flex justify-between items-center">
            <div className="h-5 bg-gray-200 rounded w-1/3"></div>
            <div className="h-6 bg-gray-200 rounded w-1/4"></div>
        </div>
    </div>
);

function ProductList({ products, loading, getProductStats, onNavigate, onSearch }) {
    const [sortOrder, setSortOrder] = useState('alpha');

    const sortedProducts = useMemo(() => {
        const productList = [...products];
        productList.sort((a, b) => {
            switch (sortOrder) {
                case 'price': return (getProductStats(a.id).minPrice ?? Infinity) - (getProductStats(b.id).minPrice ?? Infinity);
                case 'rating': return (b.avgRating ?? 0) - (a.avgRating ?? 0);
                default: return a.name.localeCompare(b.name);
            }
        });
        return productList;
    }, [products, sortOrder, getProductStats]);

    return (
        <div className="space-y-6">
            <div className="grid md:grid-cols-2 gap-4 items-center">
                <div className="relative"><input type="text" placeholder="Buscar producto..." onChange={e => onSearch(e.target.value)} className="w-full p-3 pl-10 border border-gray-300 rounded-xl focus:outline-none focus:ring-2 focus:ring-indigo-500" /><div className="absolute top-0 left-0 inline-flex items-center p-3"><SearchIcon /></div></div>
                <div className="flex items-center justify-end space-x-2"><label htmlFor="sort" className="text-sm font-medium text-gray-700">Ordenar por:</label><select id="sort" value={sortOrder} onChange={e => setSortOrder(e.target.value)} className="p-2 border border-gray-300 rounded-lg bg-white focus:outline-none focus:ring-2 focus:ring-indigo-500"><option value="alpha">Alfabético</option><option value="price">Más barato</option><option value="rating">Mejor calificado</option></select></div>
            </div>
            <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-5">
                {loading ? (
                    Array.from({ length: 6 }).map((_, i) => <ProductCardSkeleton key={i} />)
                ) : sortedProducts.length > 0 ? (
                    sortedProducts.map(product => {
                        const stats = getProductStats(product.id);
                        return (
                            <div key={product.id} onClick={() => onNavigate('productDetail', product)} className="bg-white p-4 rounded-xl shadow-md hover:shadow-xl hover:-translate-y-1 transition-all duration-200 cursor-pointer border border-gray-200 flex flex-col justify-between">
                                <div><h3 className="font-bold text-lg text-gray-800 truncate">{product.name}</h3><p className="text-gray-600 text-sm mb-2">{product.brand}</p></div>
                                <div className="mt-3 pt-3 border-t border-gray-100 flex justify-between items-center"><div className="flex items-center"><StarIcon className="h-5 w-5 text-yellow-400 mr-1" /><span className="text-sm font-bold text-gray-700">{product.avgRating ? product.avgRating.toFixed(1) : 'N/A'}</span><span className="text-xs text-gray-500 ml-1">({product.ratingCount || 0})</span></div>{stats.count > 0 ? <span className="font-bold text-green-600 text-lg">${stats.minPrice.toFixed(2)}</span> : <span className="text-sm text-gray-400">Sin precios</span>}</div>
                            </div>
                        );
                    })
                ) : (
                    <p className="text-center col-span-full text-gray-500 mt-8">No se encontraron productos. ¡Sé el primero en agregar uno!</p>
                )}
            </div>
        </div>
    );
}

const PriceTrend = ({ priceHistory }) => {
    const data = useMemo(() => priceHistory
        .map(p => ({
            date: p.timestamp.toMillis(),
            price: p.price
        }))
        .sort((a, b) => a.date - b.date)
        .map(p => ({
            ...p,
            date: new Date(p.date).toLocaleDateString('es-AR')
        })), [priceHistory]);
    
    if (data.length < 2) {
        return <div className="text-center text-gray-500 py-10">Se necesitan al menos 2 precios para mostrar una tendencia.</div>
    }

    return (
        <div className="h-64">
            <ResponsiveContainer width="100%" height="100%">
                <LineChart data={data} margin={{ top: 5, right: 20, left: -10, bottom: 5 }}>
                    <CartesianGrid strokeDasharray="3 3" stroke="#e0e0e0" />
                    <XAxis dataKey="date" fontSize={12} tickLine={false} axisLine={false} />
                    <YAxis domain={['dataMin - 1', 'dataMax + 1']} fontSize={12} tickLine={false} axisLine={false} tickFormatter={(value) => `$${value.toFixed(0)}`} />
                    <Tooltip contentStyle={{ backgroundColor: '#fff', border: '1px solid #ccc', borderRadius: '0.5rem' }} labelStyle={{ fontWeight: 'bold' }} formatter={(value) => [`$${value.toFixed(2)}`, 'Precio']} />
                    <Line type="monotone" dataKey="price" stroke="#4F46E5" strokeWidth={2} dot={{ r: 4 }} activeDot={{ r: 6 }} />
                </LineChart>
            </ResponsiveContainer>
        </div>
    );
};


function ProductDetail({ product, prices, onNavigate, user }) {
    const [newPrice, setNewPrice] = useState('');
    const [supermarket, setSupermarket] = useState('Coto');
    const [isSubmittingPrice, setIsSubmittingPrice] = useState(false);
    const [userRating, setUserRating] = useState(0);
    const [isSubmittingRating, setIsSubmittingRating] = useState(false);
    const addToast = useToast();

    useEffect(() => {
        if (!user || !product) return;
        const ratingDocRef = doc(db, `artifacts/${appId}/public/data/products/${product.id}/ratings`, user.uid);
        getDoc(ratingDocRef).then(ratingDoc => {
            setUserRating(ratingDoc.exists() ? ratingDoc.data().rating : 0);
        });
    }, [user, product]);

    const handlePriceSubmit = async (e) => {
        e.preventDefault();
        if (!newPrice || isNaN(parseFloat(newPrice))) return;
        setIsSubmittingPrice(true);
        try {
            await addDoc(collection(db, `artifacts/${appId}/public/data/prices`), { productId: product.id, userId: user.uid, price: parseFloat(newPrice), supermarket, timestamp: new Date() });
            setNewPrice('');
            addToast('¡Precio guardado con éxito!');
        } catch (error) { console.error("Error adding price: ", error); addToast('Error al guardar el precio', 'error'); }
        finally { setIsSubmittingPrice(false); }
    };
    
    const handleRatingSubmit = async (newRating) => {
        setIsSubmittingRating(true);
        const productRef = doc(db, `artifacts/${appId}/public/data/products`, product.id);
        const ratingRef = doc(db, `artifacts/${appId}/public/data/products/${product.id}/ratings`, user.uid);
        try {
            await runTransaction(db, async (t) => {
                const productDoc = await t.get(productRef);
                const userRatingDoc = await t.get(ratingRef);
                if (!productDoc.exists()) throw "El producto no existe!";
                const oldRating = userRatingDoc.exists() ? userRatingDoc.data().rating : 0;
                const { ratingCount = 0, ratingSum = 0 } = productDoc.data();
                const newRatingCount = userRatingDoc.exists() ? ratingCount : ratingCount + 1;
                const newRatingSum = ratingSum - oldRating + newRating;
                const newAvgRating = newRatingCount > 0 ? newRatingSum / newRatingCount : 0;
                t.set(ratingRef, { rating: newRating, userId: user.uid });
                t.update(productRef, { ratingSum: newRatingSum, ratingCount: newRatingCount, avgRating: newAvgRating });
            });
            setUserRating(newRating);
            addToast('¡Gracias por tu calificación!');
        } catch (e) { console.error("Error submitting rating: ", e); addToast('Error al calificar', 'error'); }
        finally { setIsSubmittingRating(false); }
    };

    const minPrice = prices.length > 0 ? Math.min(...prices.map(p => p.price)) : null;
    
    return (
        <div>
            <button onClick={() => onNavigate('home')} className="mb-4 text-indigo-500 hover:underline">&larr; Volver a la lista</button>
            <div className="bg-white p-4 md:p-6 rounded-xl shadow-lg space-y-8">
                <div>
                    <h2 className="text-3xl font-bold">{product.name}</h2>
                    <p className="text-xl text-gray-500">{product.brand}</p>
                    <div className="mt-2 flex items-center space-x-2"><StarRating rating={product.avgRating || 0} readOnly={true} /><span className="text-gray-600">({product.ratingCount || 0} calificaciones)</span></div>
                </div>

                <div className="p-4 bg-gray-50 rounded-lg">
                    <h3 className="font-bold text-lg mb-2">Historial de Precio</h3>
                    <PriceTrend priceHistory={prices} />
                </div>
                
                <div className="p-4 bg-gray-50 rounded-lg">
                    <h3 className="font-bold text-lg mb-2">Califica este producto</h3>
                    <StarRating rating={userRating} onRating={handleRatingSubmit} isSubmitting={isSubmittingRating} />
                </div>

                <div className="p-4 bg-gray-50 rounded-lg">
                    <h3 className="font-bold mb-3 text-lg">Aportá un precio</h3>
                    <form onSubmit={handlePriceSubmit} className="grid grid-cols-1 md:grid-cols-3 gap-3">
                        <input type="number" step="0.01" value={newPrice} onChange={e => setNewPrice(e.target.value)} placeholder="Ej: 1250.50" className="p-2 border rounded-md w-full" />
                        <select value={supermarket} onChange={e => setSupermarket(e.target.value)} className="p-2 border rounded-md w-full bg-white">{["Coto", "Carrefour", "Dia", "Jumbo", "Disco", "Vea", "ChangoMás", "Otro"].map(s => <option key={s} value={s}>{s}</option>)}</select>
                        <button type="submit" disabled={isSubmittingPrice} className="w-full bg-indigo-500 text-white p-2 rounded-md hover:bg-indigo-600 disabled:bg-gray-400">Guardar</button>
                    </form>
                </div>

                <div>
                    <h3 className="font-bold mb-2">Precios Registrados ({prices.length})</h3>
                    <ul className="space-y-2">{prices.sort((a,b) => a.price - b.price).map(price => (<li key={price.id} className={`p-3 rounded-lg flex justify-between items-center ${price.price === minPrice ? 'bg-green-100 border-l-4 border-green-500' : 'bg-gray-100'}`}><div><span className="font-bold text-lg">${price.price.toFixed(2)}</span><span className="ml-3 text-gray-600">{price.supermarket}</span></div><span className="text-xs text-gray-500">{new Date(price.timestamp.toDate()).toLocaleDateString()}</span></li>))}</ul>
                </div>
            </div>
        </div>
    );
}

function AddProductForm({ onNavigate }) {
    const [name, setName] = useState('');
    const [brand, setBrand] = useState('');
    const [category, setCategory] = useState('');
    const [isSubmitting, setIsSubmitting] = useState(false);
    const addToast = useToast();
    
    const handleSubmit = async (e) => {
        e.preventDefault();
        if (!name || !brand) return;
        setIsSubmitting(true);
        try {
            await addDoc(collection(db, `artifacts/${appId}/public/data/products`), { name, brand, category: category || "General", createdAt: new Date(), avgRating: 0, ratingCount: 0, ratingSum: 0 });
            addToast('Producto creado con éxito');
            onNavigate('home');
        } catch (error) { console.error("Error adding product: ", error); addToast('Error al crear producto', 'error');}
        finally { setIsSubmitting(false); }
    };

    return (
        <div className="max-w-md mx-auto">
             <button onClick={() => onNavigate('home')} className="mb-4 text-indigo-500 hover:underline">&larr; Volver</button>
            <form onSubmit={handleSubmit} className="bg-white p-8 rounded-xl shadow-lg space-y-6">
                <h2 className="text-2xl font-bold text-center text-gray-800">Agregar Nuevo Producto</h2>
                <div><label className="block text-sm font-medium text-gray-700 mb-1">Nombre</label><input type="text" value={name} onChange={e => setName(e.target.value)} placeholder="Ej: Fideos Tirabuzón" className="w-full p-2 border border-gray-300 rounded-md"/></div>
                <div><label className="block text-sm font-medium text-gray-700 mb-1">Marca</label><input type="text" value={brand} onChange={e => setBrand(e.target.value)} placeholder="Ej: Lucchetti" className="w-full p-2 border border-gray-300 rounded-md"/></div>
                <div><label className="block text-sm font-medium text-gray-700 mb-1">Categoría</label><input type="text" value={category} onChange={e => setCategory(e.target.value)} placeholder="Ej: Almacén" className="w-full p-2 border border-gray-300 rounded-md"/></div>
                <button type="submit" disabled={isSubmitting} className="w-full bg-indigo-500 text-white py-3 rounded-md hover:bg-indigo-600 disabled:bg-gray-400 font-semibold">Crear Producto</button>
            </form>
        </div>
    );
}

function SmartAnalysisView({ products, prices }) {
    const [selectedCategory, setSelectedCategory] = useState("");
    const [analysis, setAnalysis] = useState("");
    const [isLoading, setIsLoading] = useState(false);

    const categories = useMemo(() => {
        const cats = products.reduce((acc, p) => {
            if (p.category) acc.add(p.category);
            return acc;
        }, new Set());
        return ["", ...Array.from(cats).sort()];
    }, [products]);

    const callGeminiAPI = async (prompt) => {
        const apiKey = "";
        const apiUrl = `https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=${apiKey}`;
        const payload = { contents: [{ role: "user", parts: [{ text: prompt }] }] };

        const response = await fetch(apiUrl, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(payload) });
        if (!response.ok) throw new Error(`API Error: ${response.statusText}`);
        const result = await response.json();
        return result.candidates?.[0]?.content?.parts?.[0]?.text || "No se pudo obtener una respuesta.";
    };

    const handleAnalyze = async () => {
        if (!selectedCategory) return;
        setIsLoading(true); setAnalysis("");
        const categoryProducts = products.filter(p => p.category === selectedCategory);
        const dataForAnalysis = categoryProducts.map(p => {
            const productPrices = prices[p.id]?.map(price => price.price) || [];
            const avgPrice = productPrices.length > 0 ? productPrices.reduce((a, b) => a + b, 0) / productPrices.length : 0;
            return { name: `${p.name} ${p.brand}`, avgPrice: avgPrice.toFixed(2), avgRating: p.avgRating?.toFixed(2) || 'N/A', ratingCount: p.ratingCount || 0 };
        }).filter(p => p.avgPrice > 0);

        if (dataForAnalysis.length < 2) {
            setAnalysis("No hay suficientes datos en esta categoría para un análisis significativo. Se necesitan al menos dos productos con precios registrados.");
            setIsLoading(false); return;
        }

        const prompt = `Eres un experto en compras de supermercado en Argentina. Te proporcionaré datos de productos de la categoría '${selectedCategory}': precio promedio y calificación de usuarios (1-5 estrellas). Analiza y recomienda el de mejor relación precio-calidad. Explica tu elección en 2-3 párrafos. Menciona opciones 'premium' (caras pero de alta calidad) y advierte sobre las de baja calidad. Basa tu análisis en los datos. Datos: ${JSON.stringify(dataForAnalysis)}. Responde en Markdown.`;

        try { setAnalysis(await callGeminiAPI(prompt)); }
        catch (error) { console.error("Error analyzing:", error); setAnalysis("Ocurrió un error al contactar a la IA."); }
        finally { setIsLoading(false); }
    };

    return (
        <div className="bg-white p-6 rounded-xl shadow-lg space-y-6">
            <div className="text-center"><SparkleIcon className="h-10 w-10 text-indigo-500 mx-auto" /><h2 className="text-2xl font-bold text-gray-800 mt-2">Análisis de Compra Inteligente</h2><p className="text-gray-600">Deja que la IA te ayude a encontrar la mejor relación precio-calidad.</p></div>
            <div className="flex flex-col md:flex-row gap-3 items-center justify-center">
                <select value={selectedCategory} onChange={e => setSelectedCategory(e.target.value)} className="w-full md:w-auto p-3 border border-gray-300 rounded-lg bg-white focus:outline-none focus:ring-2 focus:ring-indigo-500"><option value="" disabled>Selecciona una categoría</option>{categories.map(c => c && <option key={c} value={c}>{c}</option>)}</select>
                <button onClick={handleAnalyze} disabled={!selectedCategory || isLoading} className="w-full md:w-auto bg-indigo-500 text-white font-semibold py-3 px-6 rounded-lg hover:bg-indigo-600 disabled:bg-gray-400 transition-colors">{isLoading ? "Analizando..." : "Analizar Categoría"}</button>
            </div>
            {isLoading && (<div className="flex justify-center items-center p-8"><div className="w-8 h-8 border-4 border-t-transparent border-indigo-500 rounded-full animate-spin"></div><p className="ml-4 text-gray-600">La IA está procesando los datos...</p></div>)}
            {analysis && (<div className="mt-6 p-5 bg-indigo-50 rounded-lg prose prose-sm max-w-none prose-headings:text-indigo-700"><pre className="whitespace-pre-wrap font-sans bg-transparent p-0">{analysis}</pre></div>)}
        </div>
    );
}
